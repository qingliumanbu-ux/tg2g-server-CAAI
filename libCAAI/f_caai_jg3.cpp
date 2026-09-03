/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：拆批并坯信息
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件

BM2_FUNCTION_EXPORT

int f_caai_jg3(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal price_unit;
	CString sqlstr_sub = "";
	CString plan_no = "";
	CString new_mat_no,old_mat_no;
	CString equ_no = "";
	CString mat_type = "";
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
		

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			mat_type = bcls_rec->Tables[0].Rows[i]["MAT_TYPE"].ToString(); //C拆 \B 并
			old_mat_no = bcls_rec->Tables[0].Rows[i]["OLD_MAT_NO"].ToString();
			new_mat_no = bcls_rec->Tables[0].Rows[i]["NEW_MAT_NO"].ToString();			
			wt = bcls_rec->Tables[0].Rows[i]["WT"].ToDecimal();

			//plan_no = bcls_rec->Tables[0].Rows[i]["PLAN_NO"].ToString();			

			if (old_mat_no == new_mat_no)
			{
				sqlstr = " update tcaais8 set wt=@wt,REC_REVISE_TIME =@datenow"
					" where 1=1"
					" and affirm_flag!='1'"
					" and mat_no =@mat_no"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("mat_no", old_mat_no);
				cmd_inq_1.Parameters.Set("wt", wt);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
			else
			{
				if (mat_type == "C")
				{

					sqlstr = " insert into tcaais8(REC_CREATE_TIME,mat_no,mat_thick,mat_width,mat_len,equ_no,plan_no,heat_no,sg_sign,wt)"
						" select @datenow,@new_mat_no,mat_thick,mat_width,mat_len,equ_no,plan_no,heat_no,sg_sign,@wt"
						" from tcaais8"
						" where mat_no=@old_mat_no"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("old_mat_no", old_mat_no);
					cmd_inq_1.Parameters.Set("new_mat_no", new_mat_no);
					cmd_inq_1.Parameters.Set("wt", wt);
					cmd_inq_1.Parameters.Set("datenow", datenow);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				if (mat_type == "B")
				{
					sqlstr = " DELETE from tcaais8"
						" where mat_no=@new_mat_no"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("new_mat_no", new_mat_no);
					cmd_inq_1.Parameters.Set("wt", wt);
					cmd_inq_1.Parameters.Set("datenow", datenow);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
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
