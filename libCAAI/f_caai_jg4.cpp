/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：判废后不走转发或是转储，则该量为结算量，结算费用为一半。
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件

BM2_FUNCTION_EXPORT
int f_cm_mesac2_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_caai_jg4(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal price_unit;
	CString sqlstr_sub = "";
	CString plan_no = "";
	CString mat_no = "";
	CString equ_no = "";
	CString decide_code = " ";
	CDecimal wt = 0;
	int rownum = 0;
	int fetchRowCount = 0;

	CModel tcaais7("TCAAIS7");
	CModel tcaais8("TCAAIS8");

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	try
	{
		EIClass bcls_rec1;
		EIClass bcls_ret1;

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			
			mat_no = bcls_rec->Tables[0].Rows[i]["MAT_NO"].ToString();
			//plan_no = bcls_rec->Tables[0].Rows[i]["PLAN_NO"].ToString();
			wt = bcls_rec->Tables[0].Rows[i]["WT"].ToDecimal();
			decide_code = bcls_rec->Tables[0].Rows[i]["DECIDE_CODE"].ToString();

			Log::Trace("", "", "mat_no={0},decide_code = [{1}]", mat_no, decide_code);

			sqlstr = "select count(1) from tcaais8"
				" where 1=1"
				" and affirm_flag!='1'"
				" and mat_no =@mat_no"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("mat_no", mat_no);
			fetchRowCount = cmd_inq_1.ExecuteScalar().ToInt32();
			cmd_inq_1.Close();
			if (fetchRowCount > 0)
			{
				Log::Trace("", "", "111111mat_no={0},decide_code = [{1}]", mat_no, decide_code);
				//1、更新重量
				sqlstr = " update tcaais8 set wt=@wt,decide_code=@decide_code,affirm_flag = '1',affirm_time = @datenow"
					" where 1=1"
					" and affirm_flag!='1'"
					" and mat_no =@mat_no"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("mat_no", mat_no);
				cmd_inq_1.Parameters.Set("wt", wt);
				cmd_inq_1.Parameters.Set("decide_code", decide_code);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();


				sqlstr = "select count(1) from tcaais7"
					" where 1=1"
					" and TYPE_CODE = ' '"
					" and decide_code =@decide_code"
					" and plan_no in (select plan_no from tcaais8 where mat_no=@mat_no)"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("mat_no", mat_no);
				cmd_inq.Parameters.Set("decide_code", decide_code);
				if (cmd_inq.ExecuteScalar().ToInt32() == 0) //无判废信息则新增一条
				{
					sqlstr = "insert into tcaais7(account_period, prod_date, shift_group, shift_no, plan_no,  equ_no, out_unit_code, accu_grade, recv_dept, out_task_no, proc_mode, fare_increase, cheap_range, base_price, markup_price, price_unit,mat_thick, mat_width, mat_len, tax_rate, tax_rate_name , heat_no, roll_plan_no, sg_sign, sg_std,order_no, decide_code)"
						" select account_period, prod_date, shift_group, shift_no, plan_no,  equ_no, out_unit_code, accu_grade, recv_dept, out_task_no, proc_mode, fare_increase, cheap_range, base_price, markup_price, price_unit/2,mat_thick, mat_width, mat_len, tax_rate, tax_rate_name , heat_no, roll_plan_no, sg_sign, sg_std, order_no,@decide_code"
						" from tcaais7"
						" where 1=1"
						" and TYPE_CODE = ' '"
						" and decide_code =' '"
						" and plan_no in (select plan_no from tcaais8 where mat_no=@mat_no)"
						" and rownum=1"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("mat_no", mat_no);
					cmd_inq_1.Parameters.Set("wt", wt);
					cmd_inq_1.Parameters.Set("decide_code", decide_code);
					cmd_inq_1.Parameters.Set("datenow", datenow);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				cmd_inq.Close();


				sqlstr = "select * from tcaais7"
					" where 1=1"
					" and TYPE_CODE = ' '"
					//" and decide_code =@decide_code"
					" and plan_no in (select plan_no from tcaais8 where mat_no=@mat_no)"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("mat_no", mat_no);
				cmd_inq.Parameters.Set("decide_code", decide_code);
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tcaais7.Reset();
					cmd_inq.Fetch(tcaais7);

					if (tcaais7["SEND_FLAG"].ToString() == "1"&&tcaais7["TAX_RATE_NAME"].ToString() != " ")
					{
						bcls_rec1.Tables[0].Clear();
						tcaais7.MergeTo(bcls_rec1.Tables[0]);
						if (!bcls_rec1.Tables[0].Columns.Contains("RETPO"))
						{
							bcls_rec1.Tables[0].Columns.Add(DT_STRING, "RETPO");
							bcls_rec1.Tables[0].Rows[0]["RETPO"] = "X";
						}

						doFlag = f_cm_mesac2_snd(&bcls_rec1, bcls_ret, conn);
						if (doFlag != 0)
						{
							strcpy(s.msg, "发送SAP失败!");
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					
					sqlstr = " update tcaais7 set (wt,act_wt,qty) = (select sum(wt),sum(wt),count(1) from tcaais8 where plan_no in (select plan_no from tcaais8 where mat_no=@mat_no) and affirm_flag ='1'  and decide_code = @decide_code and TYPE_CODE = ' ')"
						" where 1=1"
						" and decide_code = @decide_code"
						" and TYPE_CODE = ' '"
						" and plan_no in (select plan_no from tcaais8 where mat_no = @mat_no)"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("mat_no", mat_no);
					cmd_inq_1.Parameters.Set("decide_code", decide_code);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();

					////发送 20201231 不自动发送
					//sqlstr = " select * from tcaais7"
					//	" where TAX_RATE_NAME!=' '"
					//	" and plan_no in (select plan_no from tcaais8 where mat_no=@mat_no and decide_code = @decide_code)"
					//	" and decide_code = @decide_code"
					//	;
					//bcls_rec1.Tables[0].Clear();
					//cmd_inq_1.SetCommandText(sqlstr);
					//cmd_inq_1.Parameters.Set("mat_no", mat_no);
					//cmd_inq_1.ExecuteQuery(bcls_rec1.Tables[0]);
					//cmd_inq_1.Close();

					//if (bcls_rec1.Tables[0].Rows.get_Count() > 0)
					//{

					//	doFlag = f_cm_mesac2_snd(&bcls_rec1, bcls_ret, conn);
					//	if (doFlag != 0)
					//	{
					//		strcpy(s.msg, "发送SAP失败!");
					//		throw CApplicationException(-1, s.msg, log.Location);
					//	}
					//	sqlstr = " update tcaais7 set SEND_FLAG = '1'"
					//		",SEND_TIME = @datenow"
					//		",amt = round(price_unit*wt,2)"
					//		",CHEAP_AMT = round(cheap_range/2*wt,2)"
					//		" where 1=1"
					//		" and plan_no in (select plan_no from tcaais8 where mat_no=@mat_no and decide_code = @decide_code)"
					//		" and decide_code = @decide_code"
					//		;
					//	cmd_inq_1.SetCommandText(sqlstr);
					//	cmd_inq_1.Parameters.Set("mat_no", mat_no);
					//	cmd_inq_1.Parameters.Set("wt", wt);
					//	cmd_inq_1.Parameters.Set("datenow", datenow);
					//	cmd_inq_1.ExecuteNonQuery();
					//	cmd_inq_1.Close();
					//}

				}				
				cmd_inq.Close();
			}


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
