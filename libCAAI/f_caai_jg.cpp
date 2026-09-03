/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：根据计划号从TMMBW28取生产实绩信息，根据生产实绩从tcaac16表取加工费单价核算出加工费
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件

BM2_FUNCTION_EXPORT
int f_cm_mesac2_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_caai_jg2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn); //投入的材料做确认
int f_caai_jg(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal price_unit;
	CString sqlstr_sub = "";
	CString plan_no = "";
	CString prod_time = "",shift_no,shift_group;
	CDecimal in_wt = 0;
	int rownum = 0;

	CModel tcaais7("TCAAIS7");

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	EIClass bcls_rec2;
	EIClass bcls_ret2;

	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{

			plan_no = bcls_rec->Tables[0].Rows[i]["PLAN_NO"].ToString();
			prod_time = bcls_rec->Tables[0].Rows[i]["PLAN_END_TIME"].ToString();


			//根据委外计划号取入口材料重量
			sqlstr = " select sum(mat_wt) from HPSBWB2"
				" where plan_no=@plan_no"
				;
			in_wt = 0;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("plan_no", plan_no);
			cmd_inq_1.ExecuteReader();
			if (cmd_inq_1.Read())
			{
				in_wt = cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			sqlstr = " select mat_no,substr(mat_no,1,10) as plan_no,mat_wt as wt from HPSBWB2"
				" where plan_no=@plan_no"
				;
			bcls_rec2.Tables[0].Clear();
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("plan_no", plan_no);
			cmd_inq_1.ExecuteQuery(bcls_rec2.Tables[0]);
			cmd_inq_1.Close();			
			doFlag = f_caai_jg2(&bcls_rec2, bcls_ret, conn);
			if (doFlag != 0)
			{
				strcpy(s.msg, "更新f_caai_jg2失败!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//产出
			sqlstr = " delete from tcaais7"
				" where 1=1"
				" AND PLAN_NO = @plan_no "
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("plan_no", plan_no);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			Log::Trace("", "", "plan_no=[{0}],PLAN_END_TIME = [{1}]", plan_no, prod_time);
			f_epep_get_shift_group("BW", prod_time, shift_no, shift_group, conn);	

			//插入材料号信息
			sqlstr = " insert into tcaais8(mat_no,mat_thick,mat_width,mat_len,equ_no,plan_no,heat_no,sg_sign,wt)"
				" select  out_mat_no,mat_thick,mat_width,mat_len,equ_name,plan_no,heat_no,sg_sign,MAT_ACT_WT "
				" from TMMBW28"
				" where plan_no = @plan_no"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("plan_no", plan_no);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " select ORDER_NO,UNIT_CODE,MAT_THICK,EQU_NAME as EQU_NO, "
				" (SELECT PLAN_BACKLOG_CODE FROM HPSBWB1 WHERE plan_no = @plan_no) AS out_task_no,"
				" PROC_TYPE AS PROC_MODE,FARE_INCREASE,CHEAP_AMT as CHEAP_RANGE,RECV_DEPT ,SUM(MAT_ACT_WT) WT "
				" FROM TMMBW28 "
				" WHERE PLAN_NO = @plan_no "
				" GROUP BY ORDER_NO,UNIT_CODE,MAT_THICK,EQU_NAME,PROC_TYPE,FARE_INCREASE,CHEAP_AMT,RECV_DEPT "
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("plan_no", plan_no);
			cmd_inq.ExecuteReader();
			rownum = 0;
			if (cmd_inq.Read())
			{	
				tcaais7.Reset();
				cmd_inq.Fetch(tcaais7);
				
				rownum++;
				if (rownum == 1) //避免重复
				{
					tcaais7["IN_MAT_WT"] = in_wt;
				}
				else
				{
					tcaais7["IN_MAT_WT"] = 0;
				}
				
				tcaais7["PLAN_NO"] = plan_no;
				tcaais7["PROD_DATE"] = prod_time.SubstringNE(0, 8);	
				tcaais7["PLAN_END_TIME"] = prod_time;
				tcaais7["SHIFT_NO"] = shift_no;
				tcaais7["SHIFT_GROUP"] = shift_group;
				tcaais7["REC_CREATOR"] = s.userid;
				tcaais7["REC_CREATE_TIME"] = datenow;

				//取委外加工单位
				sqlstr = "SELECT code_desc_2_content, code_desc_4_content"
					" FROM TEP0002 WHERE 1=1"
					" AND CODE = @equ_name"
					" and CODE_CLASS = 'PSWE'"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("equ_name", tcaais7["EQU_NO"].ToString());
				cmd_inq_1.ExecuteReader();
				tcaais7["OUT_UNIT_CODE"] = " ";
				while (cmd_inq_1.Read())
				{
					tcaais7["OUT_UNIT_CODE"] = cmd_inq_1.GetString(2);
				}
				cmd_inq_1.Close();

				//取单价				
				sqlstr = " select PRICE_UNIT,TAX_RATE_NAME,TAX_RATE"
					" from tcaac16"
					" where OUT_TASK_NO = @out_task_no" //委外工序
					" and thick_min<=@mat_thick and thick_max>@mat_thick"
					" and PROC_MODE =@proc_mode" //加工类型
					" and OUT_UNIT_CODE = @out_unit_code" //加工单位
					" and RECV_DEPT = @recv_dept" //接收单位
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("out_task_no", tcaais7["OUT_TASK_NO"].ToString());
				cmd_inq_1.Parameters.Set("proc_mode", tcaais7["PROC_MODE"].ToString());
				//如果是石钢实业，则与石钢签订，委外单位也是本身
				if (tcaais7["RECV_DEPT"].ToString() == "401")
				{
					cmd_inq_1.Parameters.Set("out_unit_code", "Z6738");
				}
				else
				{				
					cmd_inq_1.Parameters.Set("out_unit_code", tcaais7["OUT_UNIT_CODE"].ToString());
				}
				cmd_inq_1.Parameters.Set("recv_dept", tcaais7["RECV_DEPT"].ToString());
				cmd_inq_1.Parameters.Set("mat_thick", tcaais7["MAT_THICK"].ToDecimal());
				cmd_inq_1.ExecuteReader();
				price_unit = 0;
				if (cmd_inq_1.Read())
				{
					tcaais7["BASE_PRICE"] = cmd_inq_1.GetDecimal(1);
					tcaais7["TAX_RATE_NAME"] = cmd_inq_1.GetString(2);
					tcaais7["TAX_RATE"] = cmd_inq_1.GetDecimal(3);
				}
				cmd_inq_1.Close();

				//取加价
				if (tcaais7["OUT_UNIT_CODE"].ToString().Trim() != "")
				{
					sqlstr = " select sum(PRICE_UNIT)"
						" from tcaac16"
						" where OUT_TASK_NO = 'J'" //委外工序
						" and thick_min<=@mat_thick and thick_max>@mat_thick"
						" and instr(@proc_mode, PROC_MODE)>0" //加价类型，模糊查询多个
						" and OUT_UNIT_CODE = @out_unit_code" //加工单位
						" and RECV_DEPT = @recv_dept" //接收单位
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("proc_mode", tcaais7["FARE_INCREASE"].ToString());
					if (tcaais7["RECV_DEPT"].ToString() == "401")
					{
						cmd_inq_1.Parameters.Set("out_unit_code", "Z6738");
					}
					else
					{
						cmd_inq_1.Parameters.Set("out_unit_code", tcaais7["OUT_UNIT_CODE"].ToString());
					}
					cmd_inq_1.Parameters.Set("recv_dept", tcaais7["RECV_DEPT"].ToString());
					cmd_inq_1.Parameters.Set("mat_thick", tcaais7["MAT_THICK"].ToDecimal());
					cmd_inq_1.ExecuteReader();
					if (cmd_inq_1.Read())
					{
						tcaais7["MARKUP_PRICE"] = cmd_inq_1.GetDecimal(1);
					}
					cmd_inq_1.Close();
				}
				
				tcaais7["PRICE_UNIT"] = tcaais7["BASE_PRICE"].ToDecimal() + tcaais7["MARKUP_PRICE"].ToDecimal() - tcaais7["CHEAP_RANGE"].ToDecimal();
				tcaais7["AMT"] = (tcaais7["WT"].ToDecimal() * tcaais7["PRICE_UNIT"].ToDecimal()).Round(2);
				tcaais7["CHEAP_AMT"] = (tcaais7["WT"].ToDecimal() * tcaais7["MARKUP_PRICE"].ToDecimal()).Round(2);

				tcaais7.Insert();
				

				//判断取下原料运费
				if (tcaais7["RECV_DEPT"].ToString() == "403"&&tcaais7["OUT_UNIT_CODE"].ToString() == "607626")
				{
					sqlstr = " select sum(mat_act_wt)"
						" from TWMA4"
						" where  1=1 "
						" and mat_no in ( select mat_no from HPSBWB2  where plan_no=@plan_no)"
						" AND FROM_STOCK_NO NOT LIKE 'W%'"
						" and stock_no = 'WZ1'"
						" AND STOCK_OPER_ORDER = '1G'  "
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("plan_no", plan_no);
					cmd_inq_1.ExecuteReader();
					if (cmd_inq_1.Read())
					{
						tcaais7["WT"] = cmd_inq_1.GetDecimal(1);
						tcaais7["MARKUP_PRICE"] = 0;
						tcaais7["BASE_PRICE"] = 0;
						tcaais7["TYPE_CODE"]= "Y";
						tcaais7["FARE_INCREASE"] = "J81";
						tcaais7["PRICE_UNIT"] = 0;
						tcaais7["AMT"] = 0;
						tcaais7["CHEAP_AMT"] = 0;
						tcaais7.Insert();
					}
					cmd_inq_1.Close();
				}
				
			}
			cmd_inq.Close();

			sqlstr = "update tcaais7 t1"
				" set (order_no, heat_no, sg_sign, sg_std, roll_plan_no) "
				" = (select order_no, heat_no, sg_sign, sg_std, roll_plan_no "
				" from HPSBWB1 t2 where t1.plan_no = t2.plan_no)"
				" where plan_no = @plan_no"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("plan_no", plan_no);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();




			//EIClass bcls_rec1;
			//EIClass bcls_ret1;

			//sqlstr = " select * from tcaais7"
			//	" where 1=1"
			//	" and SEND_FLAG!='1'"
			//	" and PRICE_UNIT!=0"
			//	" and OUT_UNIT_CODE != ' '" //加工单位
			//	" and RECV_DEPT !=' '" //接收单位
			//	" and plan_no=@plan_no"
			//	;
			//cmd_inq.SetCommandText(sqlstr);
			//cmd_inq.Parameters.Set("plan_no", plan_no);
			//cmd_inq.ExecuteQuery(bcls_rec1.Tables[0]);
			//cmd_inq.Close();

			//doFlag = f_cm_mesac2_snd(&bcls_rec1, bcls_ret, conn);
			//if (doFlag != 0)
			//{
			//	strcpy(s.msg, "发送SAP失败!");
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			////发送完成更新日期
			//sqlstr = " update tcaais7 set SEND_FLAG = '1'"
			//	",SEND_TIME = @datenow"
			//	" where 1=1"
			//	" and SEND_FLAG!='1'"
			//	" and PRICE_UNIT!=0"
			//	" and OUT_UNIT_CODE != ' '" //加工单位
			//	" and RECV_DEPT !=' '" //接收单位
			//	" and plan_no=@plan_no"
			//	;
			//cmd_inq.SetCommandText(sqlstr);
			//cmd_inq.Parameters.Set("plan_no", plan_no);
			//cmd_inq.Parameters.Set("datenow", datenow);
			//cmd_inq.ExecuteNonQuery();
			//cmd_inq.Close();



		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
