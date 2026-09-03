/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhoabin
Version:    1.0
Date:     2020-12-09
Description: 中包信息修改
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

int f_caai_jg2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
// service入口
BM2F_ENTERACE(caais7_upd)

int f_caais7_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int   doFlag = 0;
	int   fetchRowCount = 0;


	/* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaais7("TCAAIS7");

	// 数据库SQL操作字符串
	CString  sql("");
	CDbCommand cmd_inq(conn);
	CString sqlstr = "";


	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();

		for (int i = 0; i < count; i++)
		{
			tcaais7.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			//修改信息
			tcaais7["REC_REVISOR"] = s.userid;   //记录修改责任者
			tcaais7["REC_REVISE_TIME"] = datetime;   //记录修改时刻
			if (tcaais7["SEND_FLAG"].ToString().Trim() != "1")
			{				
				//取单价,如果是运费，则只取税费
				if (tcaais7["TYPE_CODE"].ToString() == "Y")
				{
					sqlstr = " select PRICE_UNIT,TAX_RATE_NAME,TAX_RATE"
						" from tcaac16"
						" where OUT_TASK_NO = @out_task_no" //委外工序						
						" and PROC_MODE =@proc_mode" //加工类型
						" and OUT_UNIT_CODE = @out_unit_code" //加工单位
						" and RECV_DEPT = @recv_dept" //接收单位
						;
				}
				else
				{
					sqlstr = " select PRICE_UNIT,TAX_RATE_NAME,TAX_RATE"
						" from tcaac16"
						" where OUT_TASK_NO = @out_task_no" //委外工序
						" and thick_min<=@mat_thick and thick_max>@mat_thick"
						" and PROC_MODE =@proc_mode" //加工类型
						" and OUT_UNIT_CODE = @out_unit_code" //加工单位
						" and RECV_DEPT = @recv_dept" //接收单位
						;
				}
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("out_task_no", tcaais7["OUT_TASK_NO"].ToString());
				cmd_inq.Parameters.Set("proc_mode", tcaais7["PROC_MODE"].ToString());
				//如果是石钢实业，则与石钢签订，委外单位也是本身
				if (tcaais7["RECV_DEPT"].ToString() == "401")
				{
					cmd_inq.Parameters.Set("out_unit_code", "Z6738");
				}
				else
				{
					cmd_inq.Parameters.Set("out_unit_code", tcaais7["OUT_UNIT_CODE"].ToString());
				}
				cmd_inq.Parameters.Set("recv_dept", tcaais7["RECV_DEPT"].ToString());
				cmd_inq.Parameters.Set("mat_thick", tcaais7["MAT_THICK"].ToDecimal());
				cmd_inq.ExecuteReader();
				tcaais7["BASE_PRICE"] = 0;
				tcaais7["TAX_RATE_NAME"] = " ";
				tcaais7["TAX_RATE"] = 0;
				if (cmd_inq.Read())
				{
					tcaais7["BASE_PRICE"] = cmd_inq.GetDecimal(1);
					tcaais7["TAX_RATE_NAME"] = cmd_inq.GetString(2);
					tcaais7["TAX_RATE"] = cmd_inq.GetDecimal(3);
				}
				else
				{
					sprintf(s.msg, "加工方式为[%s]在单价信息表中未维护存在!", (const char*)tcaais7["PROC_MODE"].ToString());
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
				cmd_inq.Close();

				sqlstr = " select sum(PRICE_UNIT)"
					" from tcaac16"
					" where OUT_TASK_NO = 'J'" //委外工序
					" and thick_min<=@mat_thick and thick_max>@mat_thick"
					" and instr(@proc_mode, PROC_MODE)>0" //加价类型，模糊查询多个
					" and OUT_UNIT_CODE = @out_unit_code" //加工单位
					" and RECV_DEPT = @recv_dept" //接收单位
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("proc_mode", tcaais7["FARE_INCREASE"].ToString());
				if (tcaais7["RECV_DEPT"].ToString() == "401")
				{
					cmd_inq.Parameters.Set("out_unit_code", "Z6738");
				}
				else
				{
					cmd_inq.Parameters.Set("out_unit_code", tcaais7["OUT_UNIT_CODE"].ToString());
				}
				cmd_inq.Parameters.Set("recv_dept", tcaais7["RECV_DEPT"].ToString());
				cmd_inq.Parameters.Set("mat_thick", tcaais7["MAT_THICK"].ToDecimal());
				cmd_inq.ExecuteReader();
				tcaais7["MARKUP_PRICE"] = 0;
				if (cmd_inq.Read())
				{
					tcaais7["MARKUP_PRICE"] = cmd_inq.GetDecimal(1);
				}
				cmd_inq.Close();

				//if (tcaais7["CHEAP_RANGE"].ToDecimal() != 0) //有优惠的
				//{
				//	tcaais7["PRICE_UNIT"] = ((tcaais7["BASE_PRICE"].ToDecimal() + tcaais7["MARKUP_PRICE"].ToDecimal()) * (100 - tcaais7["CHEAP_RANGE"].ToDecimal()) / 100).Round(2);
				//	tcaais7["AMT"] = (tcaais7["WT"].ToDecimal() * tcaais7["PRICE_UNIT"].ToDecimal() * (100 - tcaais7["CHEAP_RANGE"].ToDecimal()) / 100).Round(2);
				//	tcaais7["CHEAP_AMT"] = (tcaais7["WT"].ToDecimal() * tcaais7["PRICE_UNIT"].ToDecimal() * tcaais7["CHEAP_RANGE"].ToDecimal() / 100).Round(2);
				//}
				//else
				//{
					tcaais7["PRICE_UNIT"] = tcaais7["BASE_PRICE"].ToDecimal() + tcaais7["MARKUP_PRICE"].ToDecimal() - tcaais7["CHEAP_RANGE"].ToDecimal();
					if (tcaais7["DECIDE_CODE"].ToString() == "4")
					{
						tcaais7["PRICE_UNIT"] = ((tcaais7["BASE_PRICE"].ToDecimal() - tcaais7["CHEAP_RANGE"].ToDecimal()) / 2).Round(2);
					}
					if (tcaais7["TYPE_CODE"].ToString() == "Y")
					{
						tcaais7["BASE_PRICE"] = 0;
						tcaais7["PRICE_UNIT"] = tcaais7["BASE_PRICE"].ToDecimal() + tcaais7["MARKUP_PRICE"].ToDecimal() - tcaais7["CHEAP_RANGE"].ToDecimal();
					}
					tcaais7["AMT"] = (tcaais7["WT"].ToDecimal() * tcaais7["PRICE_UNIT"].ToDecimal()).Round(2);
					tcaais7["CHEAP_AMT"] = (tcaais7["WT"].ToDecimal() *  tcaais7["CHEAP_RANGE"].ToDecimal()).Round(2);
				//}

				//修改一条记录
				tcaais7.Update("REC_REVISOR,REC_REVISE_TIME,PROC_MODE,FARE_INCREASE,CHEAP_RANGE,PRICE_UNIT,BASE_PRICE,TAX_RATE_NAME,TAX_RATE,AMT,CHEAP_AMT,MARKUP_PRICE,REMARK,REMARK_1", "OUT_TASK_NO,OUT_UNIT_CODE,RECV_DEPT,PROD_DATE,PLAN_NO,EQU_NO,TYPE_CODE,DECIDE_CODE");
			}
			else
			{
				strcpy(s.msg, "加工费修改失败，发送标记为'1'[已发送]!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sql + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1, 1, "[%s]", s.sysmsg);
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